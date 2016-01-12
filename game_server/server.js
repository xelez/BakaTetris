var WebSocketServer = require('ws').Server
var jwt = require('jsonwebtoken');
var crypto = require('crypto')
var rabbit = require('./rabbit');
var config = require('../config');

var ip   = process.env.IP || '127.0.0.1',
    port = process.env.PORT || config.game_default_port;

var games = {};

function gen_token() {
    return crypto.randomBytes(16).toString('hex');
}

function try_parse(msg) {
    try {
        return JSON.parse(msg);
    }
    catch (e) {
        return {};
    }
}

rabbit.onrequest(function(msg) {
    var g = {
        'id' : msg.game_id,
        'first': {'user' : false, 'ws' : false},
        'second': {'user' : false, 'ws' : false},
        'create_token' : gen_token(),
        'join_token' : gen_token(),
        'state' : 'init'
    }
    games[msg.game_id] = g;
    return {
        'server_ip' : (ip+':'+port),
        'create_token' : g.create_token,
        'join_token' : g.join_token
    };
});

var wss = new WebSocketServer({ port: port });
console.log('Listenting on port = ', port);

wss.on('connection', function connection(ws) {
    console.log('connected');

    var user = false;
    var opponent = false;
    var game = false;

    function sendToOpponent(data, flags) {
        if (!opponent.ws) return;
        opponent.ws.send(data, flags, function(err) {
            if (err)
                opponent.ws.close();
        });
    }

    function handle_handshake(msg) {
        console.log('received handshake: ', msg);
        if (msg.msg_type != "handshake")
            return ws.close();
        if (!games.hasOwnProperty(msg.game_id))
            return ws.close();

        jwt.verify(msg.auth_token, config.secret, function(err, data) {
            if (err) return ws.close();

            user = data.user;
            game = games[msg.game_id];
            if (game.state != 'init')
                return ws.close();

            if (msg.game_token == game.create_token)
                handle_join_owner(user);
            else if (msg.game_token == game.join_token)
                handle_join_opponent(user);
            else
                return ws.close();

            check_game_start();
        });
    }

    function handle_join_owner(user) {
        console.log('owner joined ', user);
        game.first.ws = ws;
        game.first.user = user;
        opponent = game.second;

        rabbit.send("owner_connected", {"game_id" : game.id});
    }

    function handle_join_opponent(user) {
        console.log('opponent joined ', user);
        game.second.ws = ws;
        game.second.user = user;
        opponent = game.first;

        rabbit.send("opponent_connected", {
            "game_id" : game.id,
            "opponent" : user,
        });
    }

    function check_game_start() {
        if (!game.first.user || !game.second.user)
            return;
        console.log('game starting');
        game.state = 'started';
        delete games[game.id]; // we won't need to find again, so cleaning up

        var msg1 = {
            'msg_type' : 'opponent_connected',
            'opponent' : game.second.user,
        }
        var msg2 = {
            'msg_type' : 'opponent_connected',
            'opponent' : game.first.user,
        }
        game.first.ws.send(JSON.stringify(msg1), function(err) {
            if (err) game.first.ws.close();
        });
        game.second.ws.send(JSON.stringify(msg2), function(err) {
            if (err) game.second.ws.close();
        });
    }

    function handle_lost(msg) {
        console.log(user,' lost: ', msg);
        game.state = 'finished';
        game.winner = opponent.user;

        //delete games[game.id];

        rabbit.send("game_ended", {
            "game_id" : game.id,
            "winner" : game.winner,
        });
    }

    ws.on('message', function (data, flags) {
        var msg = {};
        if (!flags.binary) {
            console.log('parsing');
            console.log(data);
            msg = try_parse(data);
        }

        if (!user) {
            handle_handshake(msg);
            return;
        }

        if (msg.msg_type == "lost")
            handle_lost(msg);

        sendToOpponent(data, flags);
    });

    ws.on('error', function (error) {
        console.log('Error happened:');
        console.log(error);

        ws.close();
    });

    ws.on('close', function() {
        console.log('close ', user);
        if (!user) return;
        if (!game) return;
        if (game.state != 'finished') {
            handle_lost({});
            sendToOpponent('{ "msg_type": "lost" }', {});
        }
    });
});
