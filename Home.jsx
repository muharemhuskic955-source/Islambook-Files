import { useEffect, useState } from 'react';
import axios from 'axios';
import PostCard from '../components/PostCard';

export default function Home() {
  const [posts, setPosts] = useState([]);

  useEffect(() => {
    const fetchPosts = async () => {
      try {
        const res = await axios.get('/api/posts/feed');
        setPosts(res.data);
      } catch (err) {
        console.error(err);
      }
    };
    fetchPosts();
  }, []);

  return (
    <div>
      <div className="bg-white rounded-xl p-4 mb-4 shadow-sm flex items-center gap-3">
        <div className="w-10 h-10 bg-amber-400 rounded-full flex items-center justify-center">
          <span className="text-white font-bold">+</span>
        </div>
        <input
          type="text"
          placeholder="شارك آية أو دعاءً اليوم..."
          className="flex-1 border-0 focus:ring-0 bg-gray-100 rounded-full px-4 py-2"
          onClick={() => window.location.href = '/post/new'}
        />
      </div>

      {posts.map(post => (
        <PostCard key={post._id} post={post} />
      ))}
    </div>
  );
}