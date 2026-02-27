CREATE TABLE profiles (
  id UUID PRIMARY KEY REFERENCES auth.users(id),
  name VARCHAR(255),
  bio TEXT,
  profile_photo_url TEXT
);
