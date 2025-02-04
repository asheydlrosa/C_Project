const container = document.querySelector('.container');
const registerBtn = document.querySelector('.register-btn');
const loginBtn = document.querySelector('.login-btn');

// Toggle between login and register forms
registerBtn.addEventListener('click', () => {
    container.classList.add('active');
});

loginBtn.addEventListener('click', () => {
    container.classList.remove('active');
});

// Handle login form submission
const loginForm = document.querySelector(".form-box.login form");
loginForm.addEventListener("submit", (event) => {
    event.preventDefault(); // Prevent default form submission behavior

    // Example login validation (replace with real logic)
    const username = loginForm.querySelector("input[placeholder='Username']").value;
    const password = loginForm.querySelector("input[placeholder='Password']").value;

    if (username === "user" && password === "1234") { // Dummy credentials
        alert("Login successful!");
        window.location.href = "dashboard.html"; // Redirect to dashboard or other page
    } else {
        alert("Invalid login details. Please try again.");
    }
    
});

// Handle register form submission
const registerForm = document.querySelector(".form-box.register form");
registerForm.addEventListener("submit", (event) => {
    event.preventDefault(); // Prevent default form submission behavior

    // Example registration validation
    const username = registerForm.querySelector("input[placeholder='Username']").value;
    const email = registerForm.querySelector("input[placeholder='Email']").value;
    const password = registerForm.querySelector("input[placeholder='Password']").value;

    if (username && email && password) { // Basic check for non-empty fields
        alert("Registration successful!");
        window.location.href = "welcome.html"; // Redirect to welcome page or login page
    } else {
        alert("Please fill in all fields.");
    }
});
