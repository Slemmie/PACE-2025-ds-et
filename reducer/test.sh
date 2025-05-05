if [ $# -eq 0 ]; then
	echo "usage: ./test.sh <input file path> <expected output file path>"
	exit 1
fi

if [ $# -eq 1 ]; then
	input=$1

	./rel < $input > out.sol && ../verifier/verify $input out.sol && rm out.sol
else
	input=$1
	expected=$2

	./rel < $input > out.sol && ../verifier/verify $input $expected out.sol && rm out.sol
fi
