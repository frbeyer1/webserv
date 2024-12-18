
# import random

# random_number = random.randint(0, 9999999)
# print(random_number)


#!/usr/bin/env python3
import cgi
import cgitb
import os
from http import cookies
import datetime

# Enable CGI error reporting for debugging
cgitb.enable()

# Create a Cookie object
cookie = cookies.SimpleCookie()

# Set a cookie named "user_visit" with a value of "welcome"
cookie["user_visit"] = "welcome"

# Set the expiration time for the cookie (e.g., 1 hour from now)
expiration = datetime.datetime.now() + datetime.timedelta(hours=1)
cookie["user_visit"]["expires"] = expiration.strftime("%a, %d-%b-%Y %H:%M:%S GMT")

# Print the HTTP header, including the Set-Cookie header
print("Content-Type: text/html\r\n")
print(cookie.output())
print("\r\n")

# Print the HTML content
print("""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Cookie Generator</title>
</head>
<body>
    <h1>Cookie Generated</h1>
    <p>A cookie named "user_visit" has been set with the value "welcome".</p>
    <p>The cookie will expire in 1 hour.</p>
</body>
</html>
""")