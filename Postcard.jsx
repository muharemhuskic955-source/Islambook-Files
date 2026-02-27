import { FaHeart, FaComment, FaShare, FaRegBookmark } from 'react-icons/fa';
import { useState } from 'react';
import axios from 'axios';

export default function PostCard({ post }) {
  const [likes, setLikes] = useState(post.likes?.length || 0);
  const [liked, setLiked] = useState(post.likedByUser || false);

  const handleLike = async () => {
    try {
      const res = await axios.post(`/api/posts/${post._id}/like`);
      setLiked(!liked);
      setLikes(res.data.likes);
    } catch (err) {
      console.error(err);
    }
  };

  return (
    <div className="bg-white rounded-xl shadow-md overflow-hidden mb-4">
      {/* Header */}
      <div className="p-4 border-b border-gray-100 flex items-center">
        <img
          src={post.user?.avatar || `https://ui-avatars.com/api/?name=${post.user?.name}`}
          alt={post.user?.name}
          className="w-10 h-10 rounded-full"
        />
        <div className="mr-3">
          <div className="font-bold">{post.user?.name}</div>
          <div className="text-xs text-gray-500">
            {new Date(post.createdAt).toLocaleDateString('ar-SA')} • {post.location?.city || 'Online'}
          </div>
        </div>
        <div className="ml-auto text-gray-400">
          <span>⋮</span>
        </div>
      </div>

      {/* Content */}
      <div className="p-4">
        <p className="text-gray-800">{post.content}</p>
        {post.mediaUrl && (
          <div className="mt-3 rounded-lg overflow-hidden">
            <img src={post.mediaUrl} alt="post" className="w-full h-64 object-cover" />
          </div>
        )}
        {post.quranVerse && (
          <div className="mt-3 p-3 bg-amber-50 rounded-lg border-l-4 border-amber-500">
            <div className="font-serif text-lg">{post.quranVerse.text}</div>
            <div className="text-sm text-gray-600 mt-1">{post.quranVerse.sura}:{post.quranVerse.ayah} — {post.quranVerse.translation}</div>
          </div>
        )}
      </div>

      {/* Actions */}
      <div className="px-4 py-3 bg-gray-50 flex justify-between text-gray-500 text-sm">
        <button onClick={handleLike} className={`flex items-center gap-1 ${liked ? 'text-red-500' : ''}`}>
          <FaHeart size={14} />
          <span>{likes}</span>
        </button>
        <button className="flex items-center gap-1">
          <FaComment size={14} /> <span>{post.comments?.length || 0}</span>
        </button>
        <button className="flex items-center gap-1">
          <FaShare size={14} /> <span>مشاركة</span>
        </button>
        <button className="flex items-center gap-1">
          <FaRegBookmark size={14} /> <span>حفظ</span>
        </button>
      </div>
    </div>
  );
}