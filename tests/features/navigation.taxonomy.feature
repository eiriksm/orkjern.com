@api @javascript
Feature: Navigation, taxonomy
  Scenario: The navigation system is working on taxonomies
    Given I am an anonymous user
    When I am on the homepage
    And I click the selector ".field-type-taxonomy-term-reference a"
    Then selector 'span[data-property="is-server-rendered"]' should not exist.
    And selector ".view-taxonomy-term" should not exist.

