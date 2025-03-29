function print_size () {
	echo -n "$1"
	blocks=$(stat --format='%b' "$1")
	block_size=$(stat --format='%B' "$1")
	sparse_size=$(($blocks * $block_size))
	echo " sparse size: $sparse_size bytes"
}

function run_test () {
	local test_name="$1"
	local expected_result="$2"
	local actual_result="$(eval "$3")"

	echo "Test: $test_name" >> result.txt
	echo "Expected result: $expected_result" >> result.txt
	echo "Real result: $actual_result" >> result.txt
	echo "" >> result.txt
}

> result.txt

make clean


echo -n "1. Creating file...    "
truncate -s 4M fileA.bin
printf '\x01' | dd of=fileA.bin bs=1 count=1 conv=notrunc
printf '\x01' | dd of=fileA.bin bs=1 seek=$((10000)) conv=notrunc
printf '\x01' | dd of=fileA.bin bs=1 seek=$((4*1024*1024)) conv=notrunc
echo "[Done]"

make

echo -n "2. Create sparse file  "
./app fileA.bin fileB.bin
echo "[Done]"

echo -n "3. Gzipping files...   "
gzip -c fileA.bin > fileA.bin.gz
gzip -c fileB.bin > fileB.bin.gz
echo "[Done]"

echo -n "4. Extracting sparse   "
gzip -cd fileB.bin.gz | ./app fileC.bin
echo "[Done]"

echo -n "5. Nonstandard block   "
./app -b 100 fileA.bin fileD.bin
echo "[Done]"

echo "6. Stats"
print_size "fileA.bin"
print_size "fileB.bin"
print_size "fileA.bin.gz"
print_size "fileB.bin.gz"
print_size "fileC.bin"
print_size "fileD.bin"

run_test "Размер файла fileA.bin" "12K" "du -sh fileA.bin"
run_test "Размер файла fileB.bin" "12K" "du -sh fileB.bin"
run_test "Размер файла fileA.bin.gz" "8K" "du -sh fileA.bin.gz"
run_test "Размер файла fileB.bin.gz" "8K" "du -sh fileB.bin.gz"
run_test "Размер файла fileC.bin" "0K" "du -sh fileC.bin"
run_test "Размер файла fileD.bin" "16K" "du -sh fileD.bin"
