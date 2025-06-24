"""Serializers for API request/response validation and data transformation."""

import string
from django.contrib.auth.models import User
from django.contrib.auth.password_validation import validate_password
from rest_framework import serializers
from rest_framework.validators import UniqueValidator


class RegisterSerializer(serializers.ModelSerializer):
    """Serializer for user registration with password confirmation."""
    
    email = serializers.EmailField(
        required=True,
        validators=[UniqueValidator(queryset=User.objects.all())]
    )
    password = serializers.CharField(
        write_only=True,
        required=True,
        validators=[validate_password]
    )
    password2 = serializers.CharField(write_only=True, required=True)

    class Meta:
        model = User
        fields = ('username', 'password', 'password2', 'email', 'first_name', 'last_name')
        extra_kwargs = {
            'first_name': {'required': True},
            'last_name': {'required': True}
        }

    def validate(self, attrs):
        """Validate that password fields match."""
        if attrs['password'] != attrs['password2']:
            raise serializers.ValidationError({"password": "Password fields didn't match."})
        return attrs

    def create(self, validated_data):
        """Create a new user with hashed password."""
        validated_data.pop('password2')
        user = User.objects.create(
            username=validated_data['username'],
            email=validated_data['email'],
            first_name=validated_data['first_name'],
            last_name=validated_data['last_name']
        )
        user.set_password(validated_data['password'])
        user.save()
        return user


class LoginSerializer(serializers.Serializer):
    """Serializer for user login credentials."""
    
    username = serializers.CharField(required=True)
    password = serializers.CharField(required=True, write_only=True)


class LobbyCodeSerializer(serializers.Serializer):
    """Serializer for lobby code validation."""
    
    code = serializers.CharField(required=True, min_length=6, max_length=6)

    def validate_code(self, value):
        """Validate lobby code format (uppercase letters and digits only)."""
        allowed_chars = set(string.ascii_uppercase + string.digits)
        if not all(c in allowed_chars for c in value):
            raise serializers.ValidationError("Lobby code must contain only uppercase letters and digits.")
        return value
