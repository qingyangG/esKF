
"use strict";

let Gpgsv = require('./Gpgsv.js');
let Gpgsa = require('./Gpgsa.js');
let Gpgga = require('./Gpgga.js');
let Sentence = require('./Sentence.js');
let GpgsvSatellite = require('./GpgsvSatellite.js');
let Gprmc = require('./Gprmc.js');
let Gpgst = require('./Gpgst.js');

module.exports = {
  Gpgsv: Gpgsv,
  Gpgsa: Gpgsa,
  Gpgga: Gpgga,
  Sentence: Sentence,
  GpgsvSatellite: GpgsvSatellite,
  Gprmc: Gprmc,
  Gpgst: Gpgst,
};
