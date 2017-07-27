@api @javascript
Feature: No JS
  Scenario: The content should be the same, if the user uses javascript or not
    Given I am an anonymous user
    When I am on the homepage
    And I click the selector ".views-field-title a"
    # First load is ajax loaded.
    And I wait for selector ".node__content p:first-child" to appear.
    Then selector 'span[data-property="is-server-rendered"]' should not exist.
    And selector ".node__content p:first-child" should exist.
    And I remember the URL
    And I remember the text in element "#page-title" as "title"
    And I remember the text in element ".node__content" as "text"
    # THen load without ajax (manually)
    Then I go to the last rememeber URL
    Then selector 'span[data-property="is-server-rendered"]' should exist.
    And selector ".node__content p:first-child" should exist.
    Then text in element ".node__content" should equal stored text "text"
    And text in element "#page-title" should equal stored text "title"
