#!/usr/bin/env python3
import sys
import cgi
import cgitb

# Enable CGI error reporting
cgitb.enable()

def main():
    print("Content-Type: text/html\n")

    # Check if a username is provided as an argument
    if len(sys.argv) < 2:
        username = "Guest"
    else:
        username = sys.argv[1]

    # HTML template with embedded CSS
    html_template = """
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Welcome, {username}!</title>
        <style>
            body {{
                font-family: 'Arial', sans-serif;
                line-height: 1.6;
                color: #333;
                max-width: 800px;
                margin: 0 auto;
                padding: 20px;
                background-color: #f4f4f4;
            }}
            .container {{
                background-color: white;
                border-radius: 8px;
                padding: 30px;
                box-shadow: 0 0 10px rgba(0,0,0,0.1);
            }}
            h1 {{
                color: #2c3e50;
                border-bottom: 2px solid #3498db;
                padding-bottom: 10px;
            }}
            .username {{
                color: #e74c3c;
                font-weight: bold;
            }}
            .feature {{
                background-color: #ecf0f1;
                border-left: 4px solid #3498db;
                padding: 15px;
                margin-top: 20px;
            }}
            .footer {{
                margin-top: 30px;
                text-align: center;
                font-size: 0.9em;
                color: #7f8c8d;
            }}
        </style>
    </head>
    <body>
        <div class="container">
            <h1>Welcome to Your Personalized Page, <span class="username">{username}</span>!</h1>
            
            <p>We're excited to have you here. This page has been specially crafted for you.</p>
            
            <div class="feature">
                <h2>Key Features</h2>
                <ul>
                    <li>Personalized greeting</li>
                    <li>Responsive design</li>
                    <li>Modern and clean layout</li>
                </ul>
            </div>
            
            <p>Feel free to explore and enjoy your stay, {username}!</p>
            
            <div class="footer">
                <p>&copy; 2024 Fancy Website Generator. All rights reserved.</p>
            </div>
        </div>
    </body>
    </html>
    """

    # Print the formatted HTML
    print(html_template.format(username=username))

if __name__ == "__main__":
    main()