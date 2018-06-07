set -e
drush cex -y
git status
[[ -z $(git status -s) ]] || exit 1
