"""URL configuration for the battleship game application."""

from django.urls import path
from .views import (
    RegisterView, LoginView, AuthView, LogoutView,
    DashboardView, CreateLobbyView, JoinLobbyView, StartGameView, LobbyStatusView,
    GameView
)

urlpatterns = [
    path('register/', RegisterView.as_view(), name='register'),
    path('login/', LoginView.as_view(), name='login'),
    path('logout/', LogoutView.as_view(), name='logout'),
    path('dashboard/', DashboardView.as_view(), name='dashboard'),
    path('create_lobby/', CreateLobbyView.as_view(), name='create_lobby'),
    path('join_lobby/', JoinLobbyView.as_view(), name='join_lobby'),
    path('start_game/', StartGameView.as_view(), name='start_game'),
    path('lobby_status/', LobbyStatusView.as_view(), name='lobby_status'),
    path('game/<str:lobby_code>/', GameView.as_view(), name='game'),
    path('', AuthView.as_view(), name='auth'),
]
