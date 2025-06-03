
for ((i=0;i<1000;i++)); do
	./rel graph.gr solution.sol 70 128
	../rel < graph.gr > attempt.sol
	if [ $? != 0 ]; then
		echo "non-zero exit code"
		exit 1
	fi
	../../verifier/verify graph.gr solution.sol attempt.sol
	if [ $? != 0 ]; then
		echo "incorrect solution produced"
		exit 1
	fi
done
