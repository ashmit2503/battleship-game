"""Utility functions and database connections for the battleship game."""

import os
import random
import string
from pymongo import MongoClient
from django.conf import settings

# MongoDB connection setup
mongo_uri = os.environ.get('MONGO_URI', settings.MONGO_URI)
mongo_db_name = os.environ.get('MONGO_DB_NAME', settings.MONGO_DB_NAME)

client = MongoClient(mongo_uri)
db = client[mongo_db_name]
users_col = db['users']
lobbies_col = db['lobbies']


def generate_unique_lobby_code():
    """Generate a unique 6-character lobby code consisting of uppercase letters and digits.
    
    Returns:
        str: A unique lobby code that doesn't exist in the database.
    """
    while True:
        code = ''.join(random.choices(string.ascii_uppercase + string.digits, k=6))
        if not lobbies_col.find_one({"code": code}):
            return code