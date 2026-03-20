# Console Messenger

**DISCLAIMER:** This project is a final assignment for the Otus course and demonstrates the implementation of a console-based messaging system.

Use this application if you want to communicate with other users through a centralized server, exchange text messages in real-time, and store message history. Perfect for understanding client-server architecture, network programming, and database integration in C++.

## Basic Information

- **C++ 17** required (or C++20 for modern features)
- Uses **Boost.Asio** for asynchronous network operations
- **SQLite** for persistent message and user storage
- Client-server architecture with TCP/IP communication
- Messages are **persistently stored** in SQLite database

## Behaviour

- When a user sends a message, the server:
  1. Saves it to the database
  2. Checks if the recipient is online
  3. Delivers immediately if online, or stores for offline delivery
- Users receive **offline messages** upon connecting
- Message history is **loaded** when entering a chat
- Connection is **persistent** — stays open until user exits

## Features

| Feature | Status | Description |
|---------|--------|-------------|
| User registration | ✅ | Create new account with login/password |
| Authentication | ✅ | Login with existing credentials |
| Offline messages | ✅ | Messages stored and delivered when recipient connects |
| Message history | ✅ | Load previous messages from database |

## Possible Future Improvements

- **File attachments** — send images and documents
- **Message search** — find messages by content or date
- **GUI version** — port to Qt for graphical interface
- **Message reactions** — add emoji reactions to messages- **Protocol Buffers** -for efficient message serialization (compact binary format, versioning support)## Architecture

### System Context (C4 Level 1)

```mermaid
C4Context
    title Console Messenger - System Context
    
    Person(user, "User", "Person who wants to chat")
    System(messenger, "Console Messenger", "Program for exchanging text messages")
    
    Rel(user, messenger, "writes messages, reads replies")
