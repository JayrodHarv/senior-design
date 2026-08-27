# Project Plan

**Note: We can work on each step independently and then connect them together after they are done**

**Another Note: We will be using the PlatformIO extension in VSCode and writing most our code in C++.**

1. First, get the third box working (not pretty)
  - Connect microcontroller, temp sensors, display, buttons, battery, power switch, and connectors.
  - Make the third box fully functional, reading temp data and displaying it to the screen, with fully functioning power switch and toggle buttons.
  - Store 300 seconds of temp data in memory with timestamps that go with each reading

2. Make web interface with dummy data
  - Use simple html, css, javascript, with bootstrap styling
  - Fill graph with dummy data

3. Find cloud service to host api endpoint ([Supabase](https://supabase.com/) has free option)
  - Have esp32 send data to database via api endpoint
  - Have any computer request and receive data from database via api endpoint

4. Deal with Text or email notifications when temp thresholds passed
  - Opt for sending emails because it's simpler
  - Email logic exists in the cloud
  - can use Supabase again yippee!

5. Design and 3D print a container for the third box
  - or just shove it in Tupperware
