[![KapMirror](https://img.shields.io/badge/KapMirror-brightgreen.svg)](https://github.com/Chaika9/KapMirror/)
[![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://github.com/Chaika9/KapMirror/blob/main/KapMirror/Transports/Telepathy/LICENSE)

<img src="Images/Telepathy.png" title="Sylph" alt="Image"/>

Simple, message based, MMO Scale TCP networking in C++. And no magic.

- Telepathy was designed with the [KISS Principle](https://en.wikipedia.org/wiki/KISS_principle) in mind.
- Telepathy is fast and extremely reliable, designed for MMO scale Networking.
- Telepathy uses framing, so anything sent will be received the same way.
- Telepathy is raw C++ and can be used in KapEngine too.
- Telepathy is available on [GitHub](https://github.com/Chaika9/KapMirror/tree/main/KapMirror/Transports/Telepathy)
- Thanks to the original version [Github](https://github.com/vis2k/Telepathy)

## What makes Telepathy special?
Telepathy is originally Mirror's Transport Telepathy project.

We needed a library that is:
- Concurrent: Telepathy uses one thread per connection. It can make heavy use of multi core processors.
- Simple: Telepathy takes care of everything. All you need to do is call Connect/GetNextMessage/Disconnect.

MMORPGs are insanely difficult to make and we created Telepathy so that we would never have to worry about low level Networking again.
What about...
UDP vs. TCP: Minecraft and World of Warcraft are two of the biggest multiplayer games of all time and they both use TCP networking. There is a reason for that.

## Credits

vis2k & Paul - for [Telepathy](https://github.com/vis2k/Telepathy)
