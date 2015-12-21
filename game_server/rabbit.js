var amqp = require('amqplib/callback_api');
var config = require('../config');

var channel;
var request_callback = false;

amqp.connect(config.rabbit, function(err, conn) {
    conn.createChannel(function(err, ch) {
        channel = ch;
        ch.assertQueue('game_info', {durable: true});
        ch.assertQueue('rpc_queue', {durable: true});

        ch.consume('rpc_queue', function(msg) {
            var data = JSON.parse(msg.content.toString());
            var res = request_callback(data);
            var buf = new Buffer(JSON.stringify(res));
            ch.sendToQueue(msg.properties.replyTo, buf, {
                'peristent' : false,
                'correlationId' : msg.properties.correlationId
            });
            ch.ack(msg);
        }, {noAck: false});
    });
});


module.exports.onrequest = function(cb) {
    request_callback = cb;
}

module.exports.send = function(msgType, msg) {
    var buf = new Buffer(JSON.stringify(msg));
    channel.sendToQueue('game_info', buf, {
        'persistent' : true,
        'type' : msgType
    });
}
