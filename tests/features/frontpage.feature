@api
Feature: Front page
  Scenario: There should be more than 1 article on the frontpage.
    Given I am an anonymous user
    When I am on the homepage
    Then I should see "ORKJ BLOG" in the "title" region
    And I should see 10 ".views-row" elements
