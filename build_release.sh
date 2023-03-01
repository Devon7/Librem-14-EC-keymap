#!/usr/bin/bash
set -e

# use TERM to exit on error
trap "exit 1" TERM
export TOP_PID=$$

SCRATCHDIR="$(mktemp -d)"
trap 'rm -rf -- "$SCRATCHDIR"' EXIT

die () {
	local msg=$1

	if [ -n "$msg" ]; then
		echo ""
		echo -e "$msg"
		echo ""
	fi >&2
	kill -s TERM $TOP_PID
	exit 1
}

pause () {
	echo -e "\n$1"
	read -r _
}

# boards to build
boards=("librem_14")

# check release tags
TAG=$(git describe --tags --dirty)
if [[ "$TAG" == *"dirty"* ]]; then
	echo "Error: branch must be clean to perform a release build"
	exit 1
fi

# The release tag must point at HEAD - if there are extra commits after the tag,
# we'd get <tag>-<commits>-g<sha>.
if [ -z "$(git tag -l "$TAG" --points-at HEAD)" ]; then
	echo "Error: Tag doesn't point to HEAD - got description '${TAG}'" >&2
	# The solution currently is to reset the tag to point at the new HEAD
	# for RC2, RC3, etc.  Ideally we would wait to tag until the final RC
	# has passed testing though so we are not moving tags that were
	# published, this will need a bit more rework.
	echo "Reset tag to HEAD and try again" >&2
	exit 1
fi

echo "Creating new branches..."

# Create or check out a branch, and print the number of commits on that branch
# not included in origin/master.
checkout_release_branch() {
	REPO_DIR="$1"
	RELEASE_BRANCH="$2"

	if [ -n "$(git -C "$REPO_DIR" status --porcelain)" ]; then
		die "Repo $REPO_DIR is not clean, commit/stash changes and try again"
	fi
	if ! git -C "$REPO_DIR" checkout "$RELEASE_BRANCH" 2>/dev/null; then
		git -C "$REPO_DIR" checkout --detach origin/master
		git -C "$REPO_DIR" checkout -b "$RELEASE_BRANCH"
	fi
	git -C "$REPO_DIR" rev-list --count origin/master.."$RELEASE_BRANCH"
}

# Create branches in releases and utility, get the number of commits in each to
# determine the RC number
RELEASE_BRANCH="Librem-EC-$TAG"
RELEASES_RC_COMMITS="$(checkout_release_branch ../releases "$RELEASE_BRANCH")"


RC_NUM="$(("$RELEASES_RC_COMMITS" + 1))"
echo "Building $RELEASE_BRANCH/RC$RC_NUM..."

DATE=$(git show --format="%cd" --date="format:%Y-%m-%d" --no-patch)
VERSION="${TAG}_${DATE}"

for board in "${boards[@]}"
do
	filename="ec-${VERSION}.rom"
	filepath="build/purism/${board}/${VERSION}/"
	rm "${filepath}${filename}" 2>/dev/null || true

	# build board
	while ! make BOARD="purism/${board}"
	do
		read -rp "Build failed - retry?" retry
		if [[ "$retry" != "Y" && "$retry" != "y" ]] ; then
			die "user aborted"
		fi
	done

	# compress
	gzip -k "${filepath}${filename}"

	# update in releases repo
	mkdir -p "../releases/${board}/" 2>/dev/null || true
	rm "../releases/${board}/ec-"* 2>/dev/null || true
	mv "${filepath}${filename}.gz" "../releases/${board}/"

done

# Prepare commit message template
COMMITMSG_TMP="$SCRATCHDIR/commitmsg"
echo "Update Librem-EC images to $TAG/RC$RC_NUM" >"$COMMITMSG_TMP"
# For RC1, there are no prior RCs, so just use the message as-is.
if [ "$RC_NUM" -eq 1 ]; then
	COMMITMSG_ARGS=("-F" "$COMMITMSG_TMP")
else
	# For RC2+, include the changes from the prior RC, this must be edited
	# during git-commit (or git will abort)
	echo "" >>"$COMMITMSG_TMP"
	echo "<Changes from prior RC>" >>"$COMMITMSG_TMP"
	COMMITMSG_ARGS=("-t" "$COMMITMSG_TMP")
fi

# commit new boards in releases
(
	cd ../releases
	if ! git checkout "$RELEASE_BRANCH" >/dev/null 2>&1; then
		die "Error checking out release branch $RELEASE_BRANCH"
	fi
	# prompt to update changelog
	pause "Please update the releases changelog, then press enter to continue"

	# add files, do commit
	git add librem_*/ec-* >/dev/null 2>&1
	git commit -s -S -a "${COMMITMSG_ARGS[@]}"
)

pause "Ready to push $RELEASE_BRANCH/RC$RC_NUM, press enter to continue"

# Push everything last
if ! git -C ../releases push origin "$RELEASE_BRANCH" >/dev/null 2>&1; then
	echo -e "\nError pushing releases branch $TAG\n"
fi

# push branch, tag itself
BRANCH=$(git rev-parse --abbrev-ref HEAD)
if ! git push -f origin "$BRANCH" >/dev/null; then
	echo -e "\nError pushing branch $BRANCH\n"
fi
if ! git push origin "$TAG" -f >/dev/null; then
	echo -e "\nError pushing Librem-EC tag $TAG\n"
fi

echo -e "\Librem-EC release builds successfully built and branches added\n"
