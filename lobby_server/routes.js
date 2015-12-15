var express = require('express');
var router = express.Router();

var HttpError = require('./error').HttpError;

router.post('/signup', function(req, res) {
    res.send('hello');
});

router.post('/signin', function(req, res) {
    res.send('hello');
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
