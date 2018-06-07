set -e
./vendor/bin/drush cex -y
git status
[[ -z $(git status -s) ]] || exit 1
