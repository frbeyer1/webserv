#!/usr/bin/env python3
import os
import cgi

print("Content-Type: text/html\n")

print("<html><head><title>CGI Environment Variables</title></head>")
print("<body>")
print("<h1>CGI Environment Variables</h1>")
print("<table border='1'>")
print("<tr><th>Variable</th><th>Value</th></tr>")

env_vars = [
    "CONTENT_LENGTH",
    "CONTENT_TYPE",
    "AUTH_TYPE",
    "PATH_INFO",
    "PATH_TRANSLATED",
    "QUERY_STRING",
    "REMOTE_ADDR",
    "REQUEST_METHOD",
    "SCRIPT_NAME",
    "SERVER_NAME",
    "SERVER_PORT",
    "SERVER_PROTOCOL",
    "SERVER_SOFTWARE",
    "REDIRECT_STATUS"
]

for var in env_vars:
    value = os.environ.get(var, "Not set")
    print(f"<tr><td>{var}</td><td>{value}</td></tr>")

print("</table>")
print("</body></html>")