# Structure

There is a folder for each topic with the following structure:

- visual studio, only contains visual studio files, intermediate compilation files and debug builds, I have set up the solution and projects to hopefully get in the way as little as possible.

- video log, contains the video logs and the scripts for each video, scripts are written poorly and contain hints and prompts to help me when recording videos, I didn't see any reason to remove them, however I suspect they are of little value for marking.

- source, contains the full source code of all applications created for the topic, these will be seperated into a directory per application.

- data, contains all data sources used in generating my reports, except for personal images which were used on some occasions.

- bin, the visual studio solutions are setup to output release builds here, I will leave a pre-compiled executable in the folder as well.

# Considerations

Specifically for the compression program if your console beeps or starts acting strangely it's probably because you have printed some set of control characters that does something, I recommend using -b when printing to the console to avoid this as my console has exhibited some starange behaviour after dumping MB's of data in this way! Just restart the console and it should be ok..

I developed the applications on windows using visual studio 2022, if not running on linux then there may be a couple of sections that do not compile as I have not tested this, if running outside of visual studio the application relies on a couple of defines provided by visual studio although I think the only one that should matter is _WIN32