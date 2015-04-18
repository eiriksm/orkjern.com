'use strict';
var system = require('system');
casper.test.begin('The front page is reachable', function(test) {
  casper.start(system.env.TEST_URL, function() {
    // We should have that stupid lightning cloud there.
    test.assertExists('#cloudLightningFill', 'The cloud with lightning is present on the front page');
    // There should be more than one article on the front page.
    test.assert(1 < casper.evaluate(function() {
      return document.querySelectorAll('.views-field-title a').length;
    }), 'There is more than 1 article on the front page');
  }).run(function() {
    test.done();
  });
});
