#!/bin/bash

python3 -m venv venv
source venv/bin/activate
pip --isolated install --upgrade pip
pip --isolated install -r requirements.txt

echo "#### User Instructions ####"
echo "To activate type 'source venv/bin/activate'"
echo "To deactivate type 'deactivate'"
