var amqp = require('amqplib/callback_api');
var crypto = require('crypto');
var config = require('../config');

var channel;
var rpc_reply_queue;
var callbacks = {};

amqp.connect(config.rabbit, function(err, conn) {
    conn.createChannel(function(err, ch) {
        channel = ch;
        ch.assertQueue('game_info', {durable: true});
        ch.assertQueue('rpc_queue', {durable: true});

        ch.consume('game_info', function(msg) {
            var msgType = msg.properties.type;
            if (callbacks.hasOwnProperty(msgType)) {
                var data = JSON.parse(msg.content.toString());
                callbacks[msgType](null, data);
            }
            ch.ack(msg);
        }, {noAck: false})

        ch.assertQueue('', {exclusive: true}, function(err, q) {
            if (err) throw err;
            rpc_reply_queue = q.queue;

            ch.consume(q.queue, function(msg) {
                var corr = msg.properties.correlationId;
                if (callbacks.hasOwnProperty(corr)) {
                    var data = JSON.parse(msg.content.toString());
                    var cb = callbacks[corr];
                    delete callbacks[corr];
                    cb(null, data);
                }
            }, {noAck: true});
        });
    });
});

module.exports.rpc = function(msg, cb) {
    var corr = crypto.randomBytes(16).toString('hex');
    callbacks[corr] = cb;
    var message = new Buffer(JSON.stringify(msg));
    channel.sendToQueue('rpc_queue', message, {
        'persistent' : true,
        'correlationId' : corr,
        'replyTo' : rpc_reply_queue,
    });
}

module.exports.on = function(msgType, cb) {
    callbacks[msgType] = cb;
}
