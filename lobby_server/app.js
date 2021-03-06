var express    = require('express');
var bodyParser = require('body-parser');
var logger     = require('morgan');

var app = express();

var config = require('../config');
var routes = require('./routes');

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(logger('dev'));

app.use('/', routes);

// catch 404 and forward to error handler
app.use(function(req, res, next) {
  var err = new Error('Not Found');
  err.status = 404;
  next(err);
});

// error handlers

// development error handler
// will print stacktrace
if (app.get('env') === 'development') {
  app.use(function(err, req, res, next) {
    res.status(err.status || 500);
    res.json({
      message: err.message,
      error: err,
      stack: err.stack
    });
  });
}

// production error handler
// no stacktraces leaked to user
app.use(function(err, req, res, next) {
  res.status(err.status || 500);
  res.json({
    message: err.message,
  });
});


var port = process.env.PORT || config.lobby_default_port;

var server = app.listen(port, function() {
    var host = server.address().address;
    var port = server.address().port;
    
    console.log('Listening at http://%s:%s', host, port);
})
