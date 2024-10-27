-- Create the new database
CREATE DATABASE DeadlineBotDB;

-- Connect to the new database
\c DeadlineBotDB;

-- Create a new schema
CREATE SCHEMA DeadlineBotSchema;

-- Set the search path to the new schema
SET search_path TO DeadlineBotSchema;

-- Users Table
CREATE TABLE Users (
    user_id SERIAL PRIMARY KEY,
    telegram_id VARCHAR(100) UNIQUE NOT NULL
);

-- Courses Table
CREATE TABLE Courses (
    course_id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL
);

-- Enrollments Table
CREATE TABLE Enrollments (
    user_id INT NOT NULL,
    course_id INT NOT NULL,
    PRIMARY KEY (user_id, course_id),
    FOREIGN KEY (user_id) REFERENCES Users (user_id) ON DELETE CASCADE,
    FOREIGN KEY (course_id) REFERENCES Courses (course_id) ON DELETE CASCADE
);

-- Admins Table
CREATE TABLE Admins (
    user_id INT NOT NULL,
    course_id INT NOT NULL,
    PRIMARY KEY (user_id, course_id),
    FOREIGN KEY (user_id) REFERENCES Users (user_id) ON DELETE CASCADE,
    FOREIGN KEY (course_id) REFERENCES Courses (course_id) ON DELETE CASCADE
);

-- Deadlines Table
CREATE TABLE Deadlines (
    deadline_id SERIAL PRIMARY KEY,
    title VARCHAR(100) NOT NULL,
    due_date DATE NOT NULL,
    course_id INT NOT NULL,
    FOREIGN KEY (course_id) REFERENCES Courses (course_id) ON DELETE CASCADE
);

-- Terms Table
CREATE TABLE Terms (
    term_id SERIAL PRIMARY KEY,
    year INT NOT NULL,
    semester VARCHAR(10) NOT NULL,
    group_id VARCHAR(10) NOT NULL,
    course_id INT NOT NULL,
    FOREIGN KEY (course_id) REFERENCES Courses (course_id) ON DELETE CASCADE
);

-- Insert some data into Users Table
INSERT INTO Users (telegram_id) VALUES 
('user_telegram_1'), 
('user_telegram_2'), 
('user_telegram_3');

-- Insert some data into Courses Table
INSERT INTO Courses (name) VALUES 
('Database Systems'), 
('Operating Systems'), 
('Computer Networks');

-- Insert some data into Enrollments Table
INSERT INTO Enrollments (user_id, course_id) VALUES 
(2, 1), 
(2, 2), 
(3, 1), 
(3, 3);

-- Insert some data into Admins Table
INSERT INTO Admins (user_id, course_id) VALUES 
(1, 1), 
(1, 2), 
(1, 3);

-- Insert some data into Deadlines Table
INSERT INTO Deadlines (title, due_date, course_id) VALUES 
('Homework 1', '2024-05-01', 1), 
('Project 1', '2024-05-15', 2), 
('Final Exam', '2024-06-01', 3);

-- Insert some data into Terms Table
INSERT INTO Terms (year, semester, group_id, course_id) VALUES 
(2024, 'Spring', 'A1', 1), 
(2024, 'Spring', 'B1', 2), 
(2024, 'Fall', 'A2', 3);
