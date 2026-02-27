import mongoose from 'mongoose';

const userSchema = new mongoose.Schema({
  name: { type: String, required: true },
  email: { type: String, unique: true },
  password: { type: String, required: true },
  avatar: String,
  bio: String,
  country: String,
  language: { type: String, default: 'ar' },
  birthDateHijri: Date,
  isVerified: { type: Boolean, default: false },
  role: { type: String, enum: ['user', 'moderator', 'scholar', 'admin'], default: 'user' },
  privacy: { type: String, enum: ['public', 'friends', 'private'], default: 'friends' },
  createdAt: { type: Date, default: Date.now }
});

export default mongoose.model('User', userSchema);