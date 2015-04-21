'use strict';
var system = require('system');
casper.test.begin('The planet Drupal feed is reachable', function(test) {
  casper.start(system.env.TEST_URL + '/planet', function() {
    test.assertHttpStatus(200, 'The feed is reachable and does not give an error');
    this.debugPage();
  }).run(function() {
    test.done();
  });
});
