<template>
  <v-app id="inspire">
    <v-navigation-drawer v-model="drawer" fixed app clipped>
      <v-list dense>
        <v-list-tile to="/">
          <v-list-tile-action>
            <span class="iconfont icon-home"></span>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>Home</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile>
        <v-list-tile to="/wireless">
          <v-list-tile-action>
            <span class="iconfont icon-radio"></span>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>Wireless</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile>
        <v-list-tile to="/rc">
          <v-list-tile-action>
            <span class="iconfont icon-chart-bar-alt"></span>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>RcChannel</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile>
      </v-list>
    </v-navigation-drawer>
    <v-toolbar color="red accent-4" dark fixed app clipped-left>
      <v-toolbar-side-icon @click.stop="drawer = !drawer"></v-toolbar-side-icon>
      <v-toolbar-title>Snap Air Unit Home</v-toolbar-title>
    </v-toolbar>
    <v-content>
      <v-container fluid fill-height>
        <router-view></router-view>
      </v-container>
    </v-content>
    <v-footer color="red accent-4" app fixed>
      <span class="white--text">&copy; Snap Air Unit SYSTEMS CO., LTD. All rights reserved.</span>
    </v-footer>
  </v-app>
</template>

<script>
export default {
  name: "App",
  data() {
    return {
      drawer: null
    };
  }
};
</script>
