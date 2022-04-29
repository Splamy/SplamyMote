<script context="module" lang="ts">
	import type { Load } from '@sveltejs/kit';
	import { prerendering } from '$app/env';

	export const load: Load = async ({ fetch }) => {
		const json = {};

		return {
			props: {
				query: json
			}
		};
	};
</script>

<script lang="ts">
</script>

<svelte:head>
	<title>Home</title>
</svelte:head>

<h1 class="title">SplamyMote</h1>

<style lang="scss">
</style>
