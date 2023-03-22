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

# API key to access the repo
if [[ ! -f .release-api-key ]]; then
    echo "Api key not found. Configure and save an api key to .release-api-key"
    exit 1
fi

APIKEY="$(cat .release-api-key)"

if [[ -z "$APIKEY" ]]; then
    echo "Api key not found. Configure and save an api key to .release-api-key"
    exit 1
fi

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

ROM_REPO_ID=1796
TAG_COMMIT=$(git rev-list -n 1 "$TAG")
ROM_JOB=$(curl -s --header "PRIVATE-TOKEN: $(cat .release-api-key)" "https://source.puri.sm/api/v4/projects/$ROM_REPO_ID/jobs" | jq ".[] | select(.tag == true) | select(.ref == \"$TAG\")" | jq -s '.[0]')
ROM_JOB_ID=$(echo "$ROM_JOB" | jq -r ".id")
ROM_JOB_COMMIT=$(echo "$ROM_JOB" | jq -r ".commit.id")
ROM_PIPELINE_ID=$(echo "$ROM_JOB" | jq -r ".pipeline.id")

if [[ "$ROM_JOB_COMMIT" != "$TAG_COMMIT" ]]; then
    echo "Latest tag job commit doesn't match commit pointed by tag."
    echo "Job commit: $ROM_JOB_COMMIT"
    echo "Tag commit: $TAG_COMMIT"
    exit 1
fi

ISO_REPO_ID=2151
ISO_JOB_LIST=$(curl -s --header "PRIVATE-TOKEN: $(cat .release-api-key)" "https://source.puri.sm/api/v4/projects/$ISO_REPO_ID/jobs" | jq -ac '.[] | {id:.id, pipeline:{id:.pipeline.id}}')
ISO_JOB_ID="" #
for JOB in $ISO_JOB_LIST
do
    ID=$(echo "$JOB" | jq -r '.pipeline.id')
    LIBREMEC_PIPELINE_ID=$(curl -s --header "PRIVATE-TOKEN: $(cat .release-api-key)" "https://source.puri.sm/api/v4/projects/$ISO_REPO_ID/pipelines/$ID/variables" | jq -r ".[] | select(.key == \"LIBREMEC_PIPELINE_ID\") | .value")

    if [[ "$ROM_PIPELINE_ID" == "$LIBREMEC_PIPELINE_ID" ]]; then
        ISO_JOB_ID=$(echo "$JOB" | jq -r '.id')
        break
    fi
done

if [[ -z $ISO_JOB_ID ]]; then
    echo "Error: Didn't found ISO job triggered by EC job."
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
echo "Downloading $RELEASE_BRANCH/RC$RC_NUM..."

DATE=$(git show --format="%cd" --date="format:%Y-%m-%d" --no-patch "$TAG")
VERSION="${TAG}_${DATE}"

filename="ec-${VERSION}.rom"

# Download build artifact
curl -s -L --output "$filename" \
--header "PRIVATE-TOKEN: $(cat .release-api-key)" \
"https://source.puri.sm/api/v4/projects/$ROM_REPO_ID/jobs/$ROM_JOB_ID/artifacts/Librem_14_EC/$filename"

curl -s -L --output "Librem_14_EC_Update.iso" \
--header "PRIVATE-TOKEN: $(cat .release-api-key)" \
"https://source.puri.sm/api/v4/projects/$ISO_REPO_ID/jobs/$ISO_JOB_ID/artifacts/livework/Librem_14_EC_Update.iso"

# compress
gzip -k "${filename}"

# update in releases repo
mkdir -p "../releases/librem_14/" 2>/dev/null || true
rm "../releases/librem_14/ec-"* 2>/dev/null || true
mv "${filename}.gz" "../releases/librem_14/"

mv "Librem_14_EC_Update.iso" "../releases/librem_14"

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

echo -e "\Librem-EC release builds successfully built and branches added\n"
