FROM python:3.11

WORKDIR /app

# Install dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy source
COPY . /app

# Tell Django where to find settings
ENV DJANGO_SETTINGS_MODULE=backend.settings
# (if your module is named differently, adjust accordingly)

# Collect all static files into /app/staticfiles
RUN python manage.py collectstatic --noinput

# Expose and run with Gunicorn (or you can stick with runserver if you really want)
EXPOSE 8000
CMD ["gunicorn", "backend.wsgi:application", "--bind", "0.0.0.0:8000", "--workers", "3"]
