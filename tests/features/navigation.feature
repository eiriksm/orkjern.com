@api @javascript
Feature: Navigation
  Scenario: The navigation system is working
    Given I am an anonymous user
    When I am on the homepage
    And I click the selector ".views-field-title a"
    And I wait for selector ".node__content p:first-child" to appear.
    Then selector 'span[data-property="is-server-rendered"]' should not exist.
    And selector ".node__content p:first-child" should exist.

