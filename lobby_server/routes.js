var express = require('express');

var jwt = require("jsonwebtoken");
var MongoClient = require('mongodb').MongoClient
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
    res.send('hello');
});

router.post('/games', function(req, res) {
    res.send('hello');
});

router.post('/game/:id/join', function(req, res) {
    res.send('hello');
});

module.exports = router;
