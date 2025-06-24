"""Django app configuration for the battleship game application."""

from django.apps import AppConfig


class AppConfig(AppConfig):
    """Configuration for the battleship game app."""
    
    default_auto_field = 'django.db.models.BigAutoField'
    name = 'app'
