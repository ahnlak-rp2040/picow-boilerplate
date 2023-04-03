# Creating Your Project

Once you have [set up your development environment](setting-up-env.md), you can
set about creating your very own project. There are a couple of ways to do this,
but the first option is the simplest.


## Create A New Respository

Head to [github.com/ahnlak-rp2040/picow-boilerplate](https://github.com/ahnlak-rp2040/picow-boilerplate)
and you will see a green "Use this template" button near the top of the page
(if you don't, you might need to log in first).

This button gives you a couple of options, but for now choose "Create a new repository"
and give it a sensible name - like "awesome-project-1" before hitting the
"Create respository from template" button at the bottom of the page.

After some churning, you should end up looking at your shiny new repository.

Once that's done, you can simply go to your `pico` directory and fetch your
new repository:

```
git clone https://github.com/yourname/awesome-project-1
cd awesome-project-1
```


## Clone The Boilerplate

If you don't want to have your repository on Github (or if you don't have a
Github account), you can simply clone the boilerplate:

```
git clone https://github.com/ahnlak/picow-boilerplate
cd picow-boilerplate
```

(although you might want to rename the directory to `awesome-project-1` to 
avoid future confusion!)


## Configure Your Project

Whichever path you've taken, you now have a copy of this Boilerplate; before
you start adding your own code, you probably want to take a few initial steps:

* edit `CMakeLists.txt` and change the name of your project at the top of the
  file (change the `NAME` to something other than `picow-boilerplate`) - this
  defines what the `uf2` file that we'll eventually build will be called.
* include any libraries you may need - the default ones will cover most use
  cases, but once you start adding your own code you may need to add others
  (or remove redundant ones)
* check the license for your project; this boilerplate is released under the
  BSD 3-Clause License to match that used in Raspberry Pi's Pico SDK but that
  may not be suitable for your project.

Once those changes are made, you're ready to [build your project](building-projects.md);
this process will feel very familiar if you're used to cmake-based development,
but don't worry if you haven't.
