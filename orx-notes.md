Orx Notes
=========
My biggest initial hangup every time I've started was what options there were for project configuration - it's super flexible but tutorials led me to beleive I should do it a certain way that I didn't like. This is more particular to me since I'm picky about having a good template project setup. Seeing a list of possible suggestions to illustrate this flexibility would be helpful.

Another hangup was the fact that the config file is next to the executable - it would be nice to note that this can be changed in code.

The next challenge was understanding how camera and viewports interact. Seeing a lot of sample config files use display resolution for frustum dimensions led me, someone who had forgotten the nuances involved in selecting frustum size, to think that this was the norm and should be conformed to. Having graphics that show a more removed perspective of how the camera behaves in relation to objects would be helpful. Explaining what worldspace looks like in the context of orx would also help. Having a solid reference for the user to anchor their mind to is essential for this.

AutoScroll was a temporary confusion. At first config files will appear to be black magic, so the language here may suggest that the camera scrolls with X instead of configuring parallax scrolling behaviour. The Scrolling tutorial on the website explains the many nuances of the configurations for Cloud, Sky, and Camera, but the config file in the repository does not and the explanations in comments are not as descriptive as they could be.

The animation tutorial makes frames in wrong order - they increment down each column, not each row.

Assumed WatchList literally hotloaded objects at runtime - didn't occur to me that pre-existing objects wouldn't change - totally magical thinking in retrospect

The physics tutorial doesn't really explain the motivations behind there being body and bodypart - I can imagine why, but clarifying their differences and giving a brief explanation of why the properties available to them differ might be useful for some users who are unfamiliar with 2D physics systems. If these are covered elsewhere on the wiki, a link would suffice. 

In general these tutorials cover only what is needed, which is good for behrevity but bad for those who are intuitive and curious. Having links to more detailed descriptions (like on wikipedia) would add a significant level of polish to these tutorials.

This is very minor, but having prev and next links would also be nice. I only say this because some browser plugins use those as a way to streamline browsing - I use one such plugin that adds keyboard shortcuts for those. Minor annoyance. Not a huge issue.

The changing direction tutorial doesn't seem to work for idle animations. I think this is because no idle animations are defined. Show why this happens in the tutorial and fix it (much like in other sections of the tutorials). It also takes a more code-oriented approach than the official tutorial files. I think overall suasagejohnson's tutorial involves less typing; in general, I think this is what is important to strive for as a programmer. It calls into question what the advantage is to the full config approach for animations. It'd be nice to understand the pros and cons of each. The anim tutorial files are quite large and intimidating by comparison, but if there was a way to crunch that file down a bit that'd be ideal.

----- ABOVE THIS LINE HAS BEEN SHARED ----

orxpm.sh doesn't work at all on windows. needs more documentation.
Having a link to http://orx-project.org/wiki/tutorials/community/enobayram/orxhub_user on orxhub repo would be nice.
