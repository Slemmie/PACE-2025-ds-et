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
if [ -d "PACE2025-instances" ]; then
	echo "cannot complete intermediate step: 'PACE2025-instances' directory already exists"
	exit 1
fi
rm -rf ./off_100 ./off_100_lbo
mkdir ./off_100 ./off_100_lbo
git clone --depth 1 --filter=blob:none --sparse https://github.com/MarioGrobler/PACE2025-instances.git
cd PACE2025-instances
git sparse-checkout set "ds/exact"
cp ds/exact/*.gr ../off_100/
i=1
tail -n +2 3223_ds_exact.csv | head -n 100 | while IFS=, read -r _id name; do
	name="${name//$'\r'/}"
	printf -v idx "%03d" "$i"
	cp "ds/exact/$name" "../off_100_lbo/${idx}_${name}"
	i=$((i+1))
done
cd ..
rm -rf PACE2025-instances
