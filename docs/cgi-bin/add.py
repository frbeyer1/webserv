
import sys

if len(sys.argv) < 3:
    print("0")
    sys.exit(1)
try:
    num1 = int(sys.argv[1])
    num2 = int(sys.argv[2])
    result = num1 + num2
    print(result)
except ValueError:
    print("0")
    sys.exit(1)
