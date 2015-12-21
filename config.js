var cfg = {}

cfg.secret = "verysecretdevbaka";
cfg.rabbit = 'amqp://localhost';
cfg.mongo = 'mongodb://localhost:27017/bakatetris';

cfg.lobby_default_port = 3000;
cfg.game_default_port = 13555;

module.exports = cfg

