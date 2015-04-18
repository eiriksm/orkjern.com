'use strict';
var system = require('system'); 
casper.test.begin('The front page is reachable', function(test) {
  casper.start(system.env.TEST_URL, function() {
    test.assertExists('h2.node__title');
  }).run(function() {
    test.done();
  });
});
