'use strict';
var system = require('system');
var url, title, text;

var plainText = function(text) {
  return text.trim().replace(/\n/g, '').replace(/\t/g, '');
}

casper.test.begin('The page works without JavaScript as well', function(test) {
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
    this.then(function() {
      url = casper.getCurrentUrl();
      title = casper.fetchText('#page-title');
      text = casper.fetchText('.node__content');
    });
    this.then(function() {
      casper.open(url);
    });
    this.waitForSelector('div[data-quickedit-field-id]', function() {
      test.assertExist('div[data-quickedit-field-id]', 'Page was loaded via regular page load.');
      test.assertEqual(casper.getCurrentUrl(), url, 'URL is the same on non-ajax page (of course it is)');
      test.assertEqual(title.trim(), casper.fetchText('#page-title').trim(), 'Title is the same on ajax and non-ajax page');
      var text1 = plainText(text);
      var text2 = plainText(casper.fetchText('.node__content'));
      test.assertEqual(text1, text2, 'Text is the same on ajax and non-ajax page');
    });
  }).run(function() {
    test.done();
  });
});
