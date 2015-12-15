var express = require('express');
var MongoClient = require('mongodb').MongoClient
var HttpError = require('./error').HttpError;

var router = express.Router();
var db;

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
        var token = "123";
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
        
        var token = "777";
        res.json({token: token});
    });
});

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
