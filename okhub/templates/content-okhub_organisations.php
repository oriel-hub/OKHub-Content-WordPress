<?php
/**
 * Sample OKHub Organisations template for displaying content in the Twentytwelve theme. Used for both single and index/archive/search.
 *
 * @package WordPress
 * @subpackage Twenty_Twelve
 * @since Twenty Twelve 1.0
 */
?>

	<article id="post-<?php the_ID(); ?>" <?php post_class(); ?>>
		<?php if ( is_sticky() && is_home() && ! is_paged() ) : ?>
		<div class="featured-post">
			<?php _e( 'Featured post', 'twentytwelve' ); ?>
		</div>
		<?php endif; ?>
		<header class="entry-header">
			<?php the_post_thumbnail(); ?>

		<?php if ( ! is_single() ) : ?>
		<div class="entry-summary">
      <!-- The excerpt and title are imported and displayed in the preferred language if available. -->
      <!-- They might also be available in other languages. In this case, the 'title' and 'description' fields can be included
           in those additional languages by using the template function okhub_field(), in the same way as shown below with the rest of the fields. -->
			<?php the_excerpt(); ?>
		</div><!-- .entry-summary -->
		<?php else : ?>
		<div class="entry-content">
        <?php the_content( __( 'Continue reading <span class="meta-nav">&rarr;</span>', 'twentytwelve' ) ); ?>
      <!-- Example of a simple way in which these fields can be displayed. -->
			<?php if ( is_single() ) : ?>
      <div class="okhub-fields">
          <!-- Get the sources for which there is content in the post and, for each one, display the imported fields. -->
          <?php foreach (okhub_get_sources() as $source_code => $source_name) { ?>
            <h2><?php echo $source_name; ?> content</h2>
            <!-- Source fields -->
            <!-- See the file "includes/okhub.settings.inc" for the complete list of available fields.
                 The field names used in Worpdress when importing content from the OKHub can be changed in that file. -->
            <!-- The function okhub_field is used to retrieve the fields values:
                  okhub_field($field_name, $source='', $language='', $before='', $after='', $format='', $separator=', '); -->
            <ul>
              <?php okhub_field(OKHUB_WP_FN_NAME, $source_code, '', '<li class="okhub-field">' . __('Name: '), '</li>'); ?>
              <?php okhub_field(OKHUB_WP_FN_ACRONYM, $source_code, '', '<li class="okhub-field">' . __('Acronym: '), '</li>'); ?>
              <?php okhub_field(OKHUB_WP_FN_ORGANISATION_URL, $source_code, '', '<li class="okhub-field">' . __('Organisation URL: '), '</li>', 'link'); ?>
              <?php okhub_field(OKHUB_WP_FN_ORGANISATION_TYPE, $source_code, '', '<li class="okhub-field">' . __('Organisation type: '), '</li>'); ?>
              <?php okhub_field(OKHUB_WP_FN_LOCATION_COUNTRY, $source_code, '', '<li class="okhub-field">' . __('Location country: '), '</li>'); ?>
              <?php okhub_field(OKHUB_WP_FN_DATE_UPDATED, $source_code, '', '<li class="okhub-field">' . __('Updated on: '), '</li>', array('date', 'Y/m/d')); ?>
            </ul>

            <!-- Source categories -->
            <!-- Call to okhub_post_categories_source() to check if there are categories from this source for this post.
                 The return value of this function could be used to display the categories,
                 without invoking to okhub_terms(), used here to show another way of displaying the categories. -->
            <?php if (okhub_post_categories_source($source_code)) { ?>
              <ul>
                <i><?php echo "$source_name " . __('Categories:'); ?></i>
                <!-- By default, the categories will be linked to their corresponding page. -->
                <?php okhub_terms('countries', $source_code, '<li class="okhub-field">' . __('Countries: '), '</li>'); ?>
                <?php okhub_terms('regions', $source_code, '<li class="okhub-field">' . __('Regions: '), '</li>'); ?>
                <?php okhub_terms('themes', $source_code, '<li class="okhub-field">' . __('Themes: '), '</li>'); ?>
                <?php okhub_terms('documenttypes', $source_code, '<li class="okhub-field">' . __('Document Types: '), '</li>'); ?>
              </ul>
            <?php
            } //if
          } // foreach source?>
        </ul>
      </div>
			<?php endif; // is_single() ?>

			<?php wp_link_pages( array( 'before' => '<div class="page-links">' . __( 'Pages:', 'twentytwelve' ), 'after' => '</div>' ) ); ?>
		</div><!-- .entry-content -->
		<?php endif; ?>

		<footer class="entry-meta">
			<?php twentytwelve_entry_meta(); ?>
      <!-- The edit link does not make sense with content retrieved with the API, so we comment it here. We can include it in single-ids_organisations.php for imported organisations -->
			<!--?php edit_post_link( __( 'Edit', 'twentytwelve' ), '<span class="edit-link">', '</span>' ); ?-->
			<?php if ( is_singular() && get_the_author_meta( 'description' ) && is_multi_author() ) : // If a user has filled out their description and this is a multi-author blog, show a bio on their entries. ?>
				<div class="author-info">
					<div class="author-avatar">
						<?php echo get_avatar( get_the_author_meta( 'user_email' ), apply_filters( 'twentytwelve_author_bio_avatar_size', 68 ) ); ?>
					</div><!-- .author-avatar -->
					<div class="author-description">
						<h2><?php printf( __( 'About %s', 'twentytwelve' ), get_the_author() ); ?></h2>
						<p><?php the_author_meta( 'description' ); ?></p>
						<div class="author-link">
							<a href="<?php echo esc_url( get_author_posts_url( get_the_author_meta( 'ID' ) ) ); ?>" rel="author">
								<?php printf( __( 'View all posts by %s <span class="meta-nav">&rarr;</span>', 'twentytwelve' ), get_the_author() ); ?>
							</a>
						</div><!-- .author-link	-->
					</div><!-- .author-description -->
				</div><!-- .author-info -->
			<?php endif; ?>
		</footer><!-- .entry-meta -->
	</article><!-- #post -->
