"""Main URL configuration for the battleship game project."""

from django.contrib import admin
from django.urls import path, include
from app.views import AuthView

urlpatterns = [
    path('admin/', admin.site.urls),
    path('api/', include('app.urls')),
    path('', AuthView.as_view(), name='root'),
]