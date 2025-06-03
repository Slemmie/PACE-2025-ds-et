# fetch official test sets

cd "$(dirname "$0")" || exit 1

# small
rm -rf ./off_small
mkdir ./off_small
if [ -d "ds_verifier" ]; then
	echo "cannot complete intermediate step: 'ds_verifier' directory already exists"
	exit 1
fi
git clone --depth 1 --filter=blob:none --sparse https://github.com/MarioGrobler/ds_verifier.git
cd ds_verifier
git sparse-checkout set "Dominating Set Verifier/src/test/resources/testset"
cd ..
cp "ds_verifier/Dominating Set Verifier/src/test/resources/testset/"*.gr ./off_small/
cp "ds_verifier/Dominating Set Verifier/src/test/resources/testset/"*.sol ./off_small/
rm -rf ./ds_verifier

# prelimenary #100
rm -rf ./off_100
mkdir ./off_100
if [ -d "PACE2025-instances" ]; then
	echo "cannot complete intermediate step: 'PACE2025-instances' directory already exists"
	exit 1
fi
git clone --depth 1 --filter=blob:none --sparse https://github.com/MarioGrobler/PACE2025-instances.git
cd PACE2025-instances
git sparse-checkout set "ds/exact"
cd ..
cp "PACE2025-instances/ds/exact/"*.gr ./off_100/
cp "PACE2025-instances/ds/exact/"*.sol ./off_100/
rm -rf ./PACE2025-instances
