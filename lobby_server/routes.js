var express = require('express');
var jwt = require("jsonwebtoken");
var mongo = require('mongodb');
var MongoClient = mongo.MongoClient;
var ObjectID = mongo.ObjectID;
var HttpError = require('./error').HttpError;
var rabbit = require('./rabbit');
var config = require('../config');

var router = express.Router();
var db;

MongoClient.connect(config.mongo, function(err, database) {
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
        var token = jwt.sign({user:user}, config.secret);
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

        var token = jwt.sign({user:user}, config.secret);
        res.json({token: token});
    });
});

router.all('/game*', function(req, res, next) {
    var token = req.body.token;
    if (!token)
        return next(new HttpError(400));

    jwt.verify(token, config.secret, function(err, data) {
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
    db.collection('games').find(
        {'state': 'open'},
        {
            'creator' : 1,
            'server_ip' : 1,
            'join_token' : 1
        })
    .toArray(function(err, items) {
        if (err) throw err;
        res.json(items);
    });
});

router.post('/games', function(req, res) {
    var game_id = new ObjectID();

    rabbit.rpc({"game_id" : game_id.toString()}, function(err, data) {
        if (err) throw err;
        console.log('server for ', game_id, ' found: ', data);

        game = {
            '_id'          : game_id,
            'creator'      : req.user,
            'opponent'     : null,
            'server_ip'    : data.server_ip,
            'create_token' : data.create_token,
            'join_token'   : data.join_token,
            'state'        : 'init'
        };

        db.collection('games').insertOne(game, function(err, result) {
            if (err) throw err;

            res.json({
                'game_id' : game_id,
                'server_ip' : game.server_ip,
                'create_token' : game.create_token
            });
        });
    });
});

rabbit.on('owner_connected', function(err, data) {
    if (err) throw err;
    console.log('owner_connected', data);
    var id = ObjectID.createFromHexString(data.game_id);

    db.collection('games').updateOne(
        {'_id' : id, 'state' : 'init'},
        {$set : {'state' : 'open'} });
});

rabbit.on('opponent_connected', function(err, data) {
    if (err) throw err;
    console.log('opponent_connected', data);
    var id = ObjectID.createFromHexString(data.game_id);

    db.collection('games').updateOne(
        {'_id' : id, 'state' : 'open'},
        {$set : {
            'state' : 'ingame',
            'opponent': data.opponent} });
});

rabbit.on('game_ended', function(err, data) {
    if (err) throw err;
    console.log('game_ended', data);
    var id = ObjectID.createFromHexString(data.game_id);

    db.collection('games').updateOne(
        {'_id' : id, 'state' : 'open'},
        {$set : {
            'state' : 'finished',
            'winner': data.winner} });
});

module.exports = router;
