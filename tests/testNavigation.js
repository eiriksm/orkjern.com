'use strict';
var system = require('system'); 
casper.test.begin('The navigation system is working', function(test) {
  casper.start(system.env.TEST_URL, function() {
    // There should be more than one article on the front page.
    test.assert(1 < casper.evaluate(function() {
      return document.querySelectorAll('.views-field-title a').length;
    }), 'There is more than 1 article on the front page');
    // Check that we can click one of them, and it will load async.
    this.click('.views-field-title a');
    this.waitForSelector('.node__meta', function waitSuccess() {
      // To check that this was loaded via ajax, and not via going to the URL,
      // we check for a selector that only exists on full pageloads.
      test.assertDoesntExist('div[data-quickedit-field-id]', 'Page was loaded via ajax');
      test.assertExists('.node__content p:first-child');
    });
  }).run(function() {
    test.done();
  });
});
