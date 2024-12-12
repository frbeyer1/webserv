#!/usr/bin/env python3
import cgi
import cgitb
import os
import sys
from pathlib import Path

# Enable CGI error reporting
cgitb.enable()

def main():
    print("Content-Type: text/html\n")
    print("<html><body>")

    # Check if a file path is provided as an argument
    if len(sys.argv) < 2:
        print("<p>Error: No file path provided.</p>")
        print("</body></html>")
        return

    file_path = sys.argv[1]

    # Check if the file exists
    if not os.path.exists(file_path):
        print(f"<p>Error: File '{file_path}' does not exist.</p>")
        print("</body></html>")
        return

    # Define the upload directory
    upload_dir = Path("/path/to/upload/directory")

    # Create the upload directory if it doesn't exist
    upload_dir.mkdir(parents=True, exist_ok=True)

    # Get the file name from the path
    file_name = os.path.basename(file_path)

    # Define the destination path
    destination = upload_dir / file_name

    try:
        # Copy the file to the upload directory
        with open(file_path, "rb") as source_file:
            with open(destination, "wb") as dest_file:
                dest_file.write(source_file.read())

        print(f"<p>File '{file_name}' uploaded successfully.</p>")
        print(f"<p>Saved to: {destination}</p>")
    except IOError as e:
        print(f"<p>Error uploading file: {str(e)}</p>")

    print("</body></html>")

if __name__ == "__main__":
    main()