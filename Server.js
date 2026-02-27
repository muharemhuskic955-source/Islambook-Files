import express from 'express';
import mongoose from 'mongoose';
import cors from 'cors';
import dotenv from 'dotenv';
import authRoutes from './routes/auth.js';
import postRoutes from './routes/posts.js';
import groupRoutes from './routes/groups.js';
import notificationRoutes from './routes/notifications.js';
import http from 'http';
import { Server } from 'socket.io';

dotenv.config();

const app = express();
const server = http.createServer(app);
const io = new Server(server, {
  cors: { origin: "*" }
});

// Middleware
app.use(cors());
app.use(express.json());

// Routes
app.use('/api/auth', authRoutes);
app.use('/api/posts', postRoutes);
app.use('/api/groups', groupRoutes);
app.use('/api/notifications', notificationRoutes);

// Socket.IO for notifications
io.on('connection', (socket) => {
  console.log('✅ User connected:', socket.id);

  socket.on('join-room', (userId) => {
    socket.join(userId);
  });

  socket.on('send-notification', ({ to, type, message }) => {
    io.to(to).emit('receive-notification', { type, message, timestamp: new Date() });
  });

  socket.on('disconnect', () => {
    console.log('❌ User disconnected');
  });
});

// DB Connection
mongoose.connect(process.env.MONGO_URI)
  .then(() => console.log('✅ MongoDB connected'))
  .catch(err => console.error('❌ DB Error:', err));

const PORT = process.env.PORT || 5000;
server.listen(PORT, () => {
  console.log(`🌍 IslamBook backend running on port ${PORT}`);
});