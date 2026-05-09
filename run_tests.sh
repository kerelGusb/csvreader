EXEC=./csvreader

echo "[TEST 0] Example"
cat test/test00.csv
echo ""
echo "Result:"
$EXEC test/test00.csv
echo ""


echo "[TEST 1] Simple table"

echo "test01.csv:"
cat test/test01.csv
echo ""
echo "Result:"
$EXEC test/test01.csv
echo ""


echo "[TEST 2] Simple dependencies + chain formula"

echo "test02.csv:"
cat test/test02.csv
echo ""
echo "Result:"
$EXEC test/test02.csv
echo ""


echo "[TEST 3] Mixed operations"

echo "test03.csv:"
cat test/test03.csv
echo ""
echo "Result:"
$EXEC test/test03.csv
echo ""


echo "[TEST 4] Dependencies #1"

echo "test04.csv:"
cat test/test04.csv
echo ""
echo "Result:"
$EXEC test/test04.csv
echo ""


echo "[TEST 5] Dependencies #2"

echo "test05.csv:"
cat test/test05.csv
echo ""
echo "Result:"
$EXEC test/test05.csv
echo ""


echo "[TEST 6] Mixed operations with division"

echo "test06.csv:"
cat test/test06.csv
echo ""
echo "Result:"
$EXEC test/test06.csv
echo ""


echo "[TEST 7] Division by zero"
echo "Expected: ERROR: Division by zero"

echo "test07.csv:"
cat test/test07.csv
echo ""
echo "Result:"
$EXEC test/test07.csv
echo ""


echo "[TEST 8] Cycle dependency"
echo "Expected: ERROR: Cycle dependency"

echo "test08.csv:"
cat test/test08.csv
echo ""
echo "Result:"
$EXEC test/test08.csv
echo ""


echo "[TEST 9] Negative numbers"

echo "test09.csv:"
cat test/test09.csv
echo ""
echo "Result:"
$EXEC test/test09.csv
echo ""


echo "[TEST 10] Large dependency graph"

echo "test10.csv:"
cat test/test10.csv
echo ""
echo "Result:"
$EXEC test/test10.csv
echo ""


echo "[TEST 11] Row names out of order"

echo "test11.csv:"
cat test/test11.csv
echo ""
echo "Result:"
$EXEC test/test11.csv
echo ""


echo "[TEST 12] Invalid operation"

echo "test12.csv:"
cat test/test12.csv
echo ""
echo "Result:"
$EXEC test/test12.csv
echo ""
