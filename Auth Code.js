// Sign up
const signUp = async (email, password) => {
  const { user, error } = await supabase.auth.signUp({
    email,
    password,
  });
  if (error) console.error(error);
};
// Login
const login = async (email, password) => {
  const { user, error } = await supabase.auth.signIn({
    email,
    password,
  });
  if (error) console.error(error);
};
// Logout
const logout = async () => {
  const { error } = await supabase.auth.signOut();
  if (error) console.error(error);
};
