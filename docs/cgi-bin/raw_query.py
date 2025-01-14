#!/usr/bin/env python3
import os
import cgi

print("Content-Type: text/plain")
print()

query_string = os.environ.get('QUERY_STRING', '')
print(f"Raw QUERY_STRING: {query_string}")
