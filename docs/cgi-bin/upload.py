#!/usr/bin/env python3
import cgi
import cgitb
import os
from pathlib import Path

# Enable CGI error reporting
cgitb.enable()

print("Content-Type: text/html\n")
print("<html><body>")

# Create a FieldStorage object to handle the form data
form = cgi.FieldStorage()

# Check if the file was uploaded
if "fileUpload" not in form:
    print("<p>Error: No file was uploaded.</p>")
    print("</body></html>")
    exit

# Get the uploaded file
file_item = form["fileUpload"]

# Check if the file item is valid
if not file_item.filename:
    print("<p>Error: No file was selected.</p>")
    print("</body></html>")
    exit

# Define the upload directory
upload_dir = Path("docs/uploads")

# Create the upload directory if it doesn't exist
upload_dir.mkdir(parents=True, exist_ok=True)

# Get the file name
file_name = os.path.basename(file_item.filename)

# Define the destination path
destination = upload_dir / file_name

try:
    # Save the uploaded file
    with open(destination, "wb") as dest_file:
        dest_file.write(file_item.file.read())

    print(f"<p>File '{file_name}' uploaded successfully.</p>")
    print(f"<p>Saved to: {destination}</p>")
except IOError as e:
    print(f"<p>Error uploading file: {str(e)}</p>")

print("</body></html>")
