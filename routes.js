import express from 'express';
import Post from '../models/Post.js';
import User from '../models/User.js';
import auth from '../middleware/auth.js';

const router = express.Router();

// Create post
router.post('/', auth, async (req, res) => {
  try {
    const { content, mediaUrl, quranVerse } = req.body;
    const post = new Post({
      userId: req.user._id,
      content,
      mediaUrl,
      quranVerse,
      isPublic: req.user.privacy === 'public'
    });
    await post.save();
    res.status(201).json(post);
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

// Get feed
router.get('/feed', auth, async (req, res) => {
  try {
    const posts = await Post.find({ isPublic: true })
      .populate('userId', 'name avatar username')
      .sort({ createdAt: -1 })
      .limit(20);
    res.json(posts);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// Like post
router.post('/:id/like', auth, async (req, res) => {
  try {
    const post = await Post.findById(req.params.id);
    if (!post) return res.status(404).json({ error: 'Post not found' });

    const liked = post.likes.includes(req.user._id);
    if (liked) {
      post.likes.pull(req.user._id);
    } else {
      post.likes.push(req.user._id);
    }
    await post.save();

    res.json({ likes: post.likes.length, liked: !liked });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

export default router;