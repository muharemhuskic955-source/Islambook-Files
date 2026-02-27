import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import Home from './pages/Home';
import Profile from './pages/Profile';
import Groups from './pages/Groups';
import PostCreate from './pages/PostCreate';
import Navbar from './components/Navbar';
import Sidebar from './components/Sidebar';

export default function App() {
  return (
    <Router>
      <div className="min-h-screen bg-gray-50">
        <Navbar />
        <div className="flex">
          <Sidebar />
          <main className="flex-1 p-4 md:p-6 max-w-4xl mx-auto w-full">
            <Routes>
              <Route path="/" element={<Home />} />
              <Route path="/profile/:id" element={<Profile />} />
              <Route path="/groups" element={<Groups />} />
              <Route path="/post/new" element={<PostCreate />} />
            </Routes>
          </main>
        </div>
      </div>
    </Router>
  );
}