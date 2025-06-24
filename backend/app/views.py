"""API views for the battleship game application."""

from django.shortcuts import render, redirect
from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.models import User
from django.conf import settings
from rest_framework.views import APIView
from rest_framework.response import Response
from rest_framework import status
from rest_framework.permissions import IsAuthenticated

from .serializers import RegisterSerializer, LoginSerializer, LobbyCodeSerializer
from .utils import lobbies_col, generate_unique_lobby_code


class AuthView(APIView):
    """View for authentication page."""
    
    def get(self, request):
        """Render authentication page."""
        return render(request, 'auth.html')


class RegisterView(APIView):
    """Handle user registration."""
    
    def get(self, request):
        """Render registration page."""
        return render(request, 'auth.html')

    def post(self, request):
        """Register a new user."""
        serializer = RegisterSerializer(data=request.data)
        if serializer.is_valid():
            serializer.save()
            return Response({
                "message": "User registered successfully",
                "user": serializer.data
            }, status=status.HTTP_201_CREATED)
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)


class LoginView(APIView):
    """Handle user login."""
    
    def get(self, request):
        """Render login page."""
        return render(request, 'auth.html')

    def post(self, request):
        """Authenticate user and create session."""
        serializer = LoginSerializer(data=request.data)
        if serializer.is_valid():
            username = serializer.validated_data['username']
            password = serializer.validated_data['password']
            user = authenticate(request, username=username, password=password)

            if user is not None:
                login(request, user)
                return Response({
                    "message": "Login successful",
                    "redirect": "/api/dashboard/"
                }, status=status.HTTP_200_OK)
            return Response({
                "message": "Invalid credentials"
            }, status=status.HTTP_401_UNAUTHORIZED)
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)


class DashboardView(APIView):
    """Display user dashboard."""
    
    permission_classes = [IsAuthenticated]

    def get(self, request):
        """Render dashboard page."""
        return render(request, 'dashboard.html')


class CreateLobbyView(APIView):
    """Create a new game lobby."""
    
    permission_classes = [IsAuthenticated]

    def post(self, request):
        """Create a new lobby with unique code."""
        code = generate_unique_lobby_code()

        lobby = {
            "code": code,
            "players": [request.user.id],
            "status": "waiting"
        }

        lobbies_col.insert_one(lobby)

        return Response({
            "message": "Lobby created successfully",
            "code": code
        }, status=status.HTTP_201_CREATED)


class JoinLobbyView(APIView):
    """Join an existing game lobby."""
    
    permission_classes = [IsAuthenticated]

    def post(self, request):
        """Join a lobby by code."""
        serializer = LobbyCodeSerializer(data=request.data)
        if serializer.is_valid():
            code = serializer.validated_data['code']

            lobby = lobbies_col.find_one({"code": code})

            if not lobby:
                return Response({
                    "message": "Lobby not found"
                }, status=status.HTTP_404_NOT_FOUND)

            if lobby["status"] != "waiting":
                return Response({
                    "message": "Game already in progress"
                }, status=status.HTTP_400_BAD_REQUEST)

            if len(lobby["players"]) >= 2:
                return Response({
                    "message": "Lobby is full"
                }, status=status.HTTP_400_BAD_REQUEST)

            lobbies_col.update_one(
                {"code": code},
                {
                    "$push": {"players": request.user.id},
                    "$set": {"status": "ready"}
                }
            )

            host_id = lobby["players"][0]
            host_user = User.objects.get(id=host_id)

            return Response({
                "message": "Successfully joined lobby",
                "code": code,
                "host": host_user.username
            }, status=status.HTTP_200_OK)

        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)


class StartGameView(APIView):
    """Start a game from a ready lobby."""
    
    permission_classes = [IsAuthenticated]

    def post(self, request):
        """Start the game if conditions are met."""
        serializer = LobbyCodeSerializer(data=request.data)
        if serializer.is_valid():
            code = serializer.validated_data['code']

            lobby = lobbies_col.find_one({"code": code})

            if not lobby:
                return Response({
                    "message": "Lobby not found"
                }, status=status.HTTP_404_NOT_FOUND)

            if request.user.id not in lobby["players"]:
                return Response({
                    "message": "You are not in this lobby"
                }, status=status.HTTP_403_FORBIDDEN)

            if lobby["status"] != "ready":
                return Response({
                    "message": "Lobby is not ready to start"
                }, status=status.HTTP_400_BAD_REQUEST)

            lobbies_col.update_one(
                {"code": code},
                {"$set": {"status": "in_progress"}}
            )

            return Response({
                "message": "Game started"
            }, status=status.HTTP_200_OK)

        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)


class LobbyStatusView(APIView):
    """Get current lobby status and player information."""
    
    permission_classes = [IsAuthenticated]

    def get(self, request):
        """Return lobby status and player information."""
        code = request.query_params.get('code', None)
        if not code:
            return Response({
                "message": "Lobby code is required"
            }, status=status.HTTP_400_BAD_REQUEST)

        lobby = lobbies_col.find_one({"code": code})

        if not lobby:
            return Response({
                "message": "Lobby not found"
            }, status=status.HTTP_404_NOT_FOUND)

        if request.user.id not in lobby["players"]:
            return Response({
                "message": "You are not in this lobby"
            }, status=status.HTTP_403_FORBIDDEN)

        player_info = []
        opponent = None

        for player_id in lobby["players"]:
            try:
                user = User.objects.get(id=player_id)
                player_info.append({
                    "id": player_id,
                    "username": user.username
                })
                if player_id != request.user.id:
                    opponent = user.username
            except User.DoesNotExist:
                pass

        return Response({
            "status": lobby["status"],
            "players": player_info,
            "opponent": opponent
        }, status=status.HTTP_200_OK)


class LogoutView(APIView):
    """Handle user logout."""
    
    permission_classes = [IsAuthenticated]

    def get(self, request):
        """Log out user and redirect to login."""
        logout(request)
        return redirect('/api/login/')


class GameView(APIView):
    """Display the game interface."""
    
    permission_classes = [IsAuthenticated]

    def get(self, request, lobby_code):
        """Render game page if user is authorized."""
        lobby = lobbies_col.find_one({"code": lobby_code})

        if not lobby or request.user.id not in lobby["players"]:
            return redirect('/api/dashboard/')

        return render(request, 'game.html', {
            'lobby_code': lobby_code,
            'websocket_url': settings.WEBSOCKET_URL
        })
