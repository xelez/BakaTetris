var express = require('express');

var jwt = require("jsonwebtoken");
var mongo = require('mongodb');

var MongoClient = mongo.MongoClient;
var ObjectID = mongo.ObjectID;
var HttpError = require('./error').HttpError;

var router = express.Router();
var db;

var secret = "verysecretdevbaka";

MongoClient.connect("mongodb://localhost:27017/bakatetris", function(err, database) {
    if (err) throw err;
    db = database;
    db.collection("users").ensureIndex("user", {unique: true}, function(err, index) {
        if (err) throw err;
    });
})

router.post('/signup', function(req, res, next) {
    var collection = db.collection("users");
    var user = req.body.user || "";
    var passwd = req.body.password || "";
    if (!user || !passwd)
        return next(new HttpError(400));
    collection.insertOne({'user': user, 'passwd': passwd}, function(err, result) {
        if (err)
            return next(new HttpError(400, "User already exists"));
            // TODO: change error code
        var token = jwt.sign({user:user}, secret);
        res.json({token: token});
    });
});

router.post('/signin', function(req, res, next) {
    var collection = db.collection("users");
    var user = req.body.user || "";
    var passwd = req.body.password || "";
    
    if (!user || !passwd)
        return next(new HttpError(400));
    
    collection.findOne({'user': user}, function(err, item) {
        if (err || !item)
            return next(new HttpError(404));
        if (item.passwd != passwd)
            return next(new HttpError(404));
        
        var token = jwt.sign({user:user}, secret);
        res.json({token: token});
    });
});

router.all('/game*', function(req, res, next) {
    var token = req.body.token;
    if (!token)
        return next(new HttpError(400));
    
    jwt.verify(token, secret, function(err, data) {
        if (err)
            return next(new HttpError(403));
        req.user = data.user;
        next();
    });
})

router.get('/games', function(req, res, next) {
    return next(new HttpError(501)); // Not implemented
});

router.get('/games/open', function(req, res) {
    db.collection('games').find({'state': 'open'}, {'creator': 1}).toArray(function(err, items) {
        if (err) throw err;
        res.json(items);
    });
});

router.post('/games', function(req, res) {
    var server_ip = '127.0.0.1:3001';

    game = {
        'creator'  : req.user,
        'opponent' : null,
        'server'   : server_ip,
        'state'    : 'open',
    };

    db.collection('games').insertOne(game, function(err, result) {
        if (err) throw err;
        var game_id = result.ops[0]._id;

        var token = jwt.sign({
            'user' : req.user,
            'server' : server_ip,
            'game_id' : game_id,
            'act' : 'create'
        }, secret);

        res.json({
            'game_id' : game_id,
            'game_server_ip' : server_ip,
            'create_game_token' : token
        });
    });
});

router.post('/game/:id/join', function(req, res, next) {
    var id = ObjectID.createFromHexString(req.params.id);
    
    db.collection('games').findOneAndUpdate(
        {'_id' : id, 'state' : 'open'},
        {$set : {'state' : 'opponent_found', 'opponent': req.user} },
        function(err, r) {
            if (err || !r.value)
                return next(new HttpError(404));

            var game = r.value;
            var token = jwt.sign({
                'user' : req.user,
                'server' : game.server,
                'game_id' : game._id,
                'act' : 'join'
            }, secret);

            res.json({
                'game_server_ip' : game.server,
                'join_game_token' : token
            });
    });
});

module.exports = router;
