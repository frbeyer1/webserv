#!/usr/bin/env python3
import cgi

# This Script expects two numbers defined in the Query
# like this: cgi-bin/add.py?x=2&y=1

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields

num1 = int(form.getvalue('x', '0'))
num2 = int(form.getvalue('y', '0'))

# Calculate the sum
total = num1 + num2

# Print the HTTP header
print("Content-type: text/html\n")

# Print the HTML content
print("<html>")
print("<head><title>Addition Result</title></head>")
print("<body>")
print(f"<h2>The sum of {num1} and {num2} is: {total}</h2>")
print("</body>")
print("</html>")
